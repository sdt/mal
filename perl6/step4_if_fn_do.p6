#!/usr/bin/env perl6

use v6;
use Core;
use Printer;
use Reader;
use ReadLine;
use Types;

my @library =
    '(def! not (fn* (a) (if a false true)))'
    ;

sub MAIN() {
    my $repl-env = malEnv.new;
    install-core($repl-env);
    for @library { rep($_, $repl-env) }

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
    PRINT(EVAL(READ($input), $env));
}

sub READ(Str $input) {
    return read-str($input);
}

sub EVAL(malValue $ast, malEnv $env) {
    unless ($ast ~~ malList) && ($ast.value.elems > 0) {
        return eval-ast($ast, $env);
    }

    my %special =
        'def!' => sub (malSymbol $sym, malValue $def) {
            $env.set($sym.value, EVAL($def, $env))
        },
        'do' => sub (*@values) {
            my $ret;
            for @values -> $value {
                $ret = EVAL($value, $env);
            }
            return $ret;
        },
        'if' => sub (malValue $cond, malValue $then, malValue $else = malNil) {
            my $ret = is-true(EVAL($cond, $env)) ?? $then !! $else;
            return EVAL($ret, $env);
        },
        'fn*' => sub (malSequence $args, malValue $body) {
            return malLambda.new($args, $body, $env);
        },
        'let*' => sub (malSequence $bindings, malValue $expr) {
            my $inner = malEnv.new(:outer($env));
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
        my ($op, @args) = eval-ast($ast, $env).value.list;
        if $op ~~ malBuiltIn {
            return $op.value.(|@args);
        }
        if $op ~~ malLambda {
            my $inner = malEnv.new(:outer($op.env));
            $inner.bind($op.args, @args);
            return EVAL($op.value, $inner);
        }
        die RuntimeError.new(pr-str($op, True) ~ " is not applicable");
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
