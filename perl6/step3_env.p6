#!/usr/bin/env perl6

use v6;
use Printer;
use Reader;
use ReadLine;
use Types;

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
                say %*ENV<MAL_DEBUG>:exists ?? $_ !! $_.message;
            }
        }
    }
}

sub rep(Str $input, $env) {
    malPRINT(malEVAL(malREAD($input), $env));
}

sub malREAD(Str $input) {
    return read-str($input);
}

sub malEVAL(malValue $ast, malEnv $env) {
    unless ($ast ~~ malList) && ($ast.value.elems > 0) {
        return eval-ast($ast, $env);
    }

    my %special =
        'def!' => sub (malSymbol $sym, malValue $def) {
            $env.set($sym.value, malEVAL($def, $env))
        },
        'let*' => sub (malSequence $bindings, malValue $expr) {
            my $inner = malEnv.new(outer => $env);
            for $bindings.value.list -> malSymbol $symbol, malValue $value {
                $inner.set($symbol.value, malEVAL($value, $inner));
            }
            return malEVAL($expr, $inner);
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

sub malPRINT($ast) {
    return pr-str($ast, True);
}

sub eval-ast(malValue $ast, malEnv $env) {
    given ($ast) {
        when malSymbol {
            return $env.get($ast.value);
        }
        when malHash {
            my %value = $ast.value.pairs.map(
                { $_.key => malEVAL($_.value, $env) });
            return malHash.new(%value);
        }
        when malList {
            my @value = $ast.value.map({ malEVAL($_, $env) });
            return malList.new(@value);
        }
        when malVector {
            my @value = $ast.value.map({ malEVAL($_, $env) });
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
