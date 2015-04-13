#!/usr/bin/env perl6

use v6;
use Environment;
use Printer;
use Reader;
use ReadLine;
use Types;

class RuntimeError is malException { }

sub MAIN() {
    my $repl-env = malEnv.new;
    $repl-env.set('+', wrap-int-op(&[+]));
    $repl-env.set('-', wrap-int-op(&[-]));
    $repl-env.set('*', wrap-int-op(&[*]));
    $repl-env.set('/', wrap-int-op(&[/]));

    while defined (my $input = read-line('user> ')) {
        say rep($input, $repl-env);

        CATCH {
            when Reader::EmptyInput {
                # nothing
            }
            default {
                say $_.message;
            }
        }
    }
}

sub rep(Str $input, $env) {
    PRINT(EVAL(READ($input), $env));
}

sub READ(Str $input) {
    return read-str($input);
}

sub EVAL(malValue $ast, malEnv $env) {
    unless $ast ~~ malList {
        return eval-ast($ast, $env);
    }

    my %special =
        'def!' => sub (malSymbol $sym, malValue $def) {
            $env.set($sym.value, EVAL($def, $env))
        },
        'let*' => sub (malSequence $bindings, malValue $expr) {
            my $inner = malEnv.new(outer => $env);
            for $bindings.value.list -> malSymbol $symbol, malValue $value {
                $inner.set($symbol.value, EVAL($value, $inner));
            }
            return EVAL($expr, $inner);
        },
        ;

    my ($op, @args) = $ast.value.list;
    if $op ~~ malSymbol && %special{$op.value} -> $handler {
        return $handler(|@args);
        CATCH {
            die RuntimeError.new($op.value ~ ': ' ~ $_);
        }
    }
    else {
        my ($builtin, @args) = eval-ast($ast, $env).value.list;
        return $builtin.value.(|@args);
    }
}

sub PRINT($ast) {
    return pr-str($ast, True);
}

sub eval-ast(malValue $ast, malEnv $env) {
    given ($ast) {
        when malSymbol {
            return $env.get($ast.value);
        }
        when malHash {
            my %value = $ast.value.pairs.map(
                { $_.key => EVAL($_.value, $env) });
            return malHash.new(%value);
        }
        when malList {
            my @value = $ast.value.map({ EVAL($_, $env) });
            return malList.new(@value);
        }
        when malVector {
            my @value = $ast.value.map({ EVAL($_, $env) });
            return malVector.new(@value);
        }
        default {
            return $ast;
        }
    }
}
sub wrap-int-op($native-func) {
    return malBuiltIn.new(sub (malInteger $a, malInteger $b) {
        my $value = $native-func($a.value, $b.value);
        return malInteger.new($value.Int);
    });
}
