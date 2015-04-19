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
                say $_;
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

sub EVAL(malValue $ast is copy, malEnv $env is copy) {

    my %special =
        'def!' => sub (malSymbol $sym, malValue $def) {
            $env.set($sym.value, EVAL($def, $env))
        },
        'fn*' => sub (malSequence $args, malValue $body) {
            return malLambda.new($args, $body, $env);
        },
        ;

    my %special-tailrec =
        'do' => sub (*@values) {
            my $ret;
            my $n = @values.elems;
            for @values[^($n-1)] -> $value {
                EVAL($value, $env);
            }
            return (@values[$n-1], $env);
        },
        'if' => sub (malValue $cond, malValue $then, malValue $else = malNil) {
            my $ast = is-true(EVAL($cond, $env)) ?? $then !! $else;
            return ($ast, $env);
        },
        'let*' => sub (malSequence $bindings, malValue $expr) {
            my $inner = malEnv.new(:outer($env));
            for $bindings.value.list -> malSymbol $symbol, malValue $value {
                $inner.set($symbol.value, EVAL($value, $inner));
            }
            return ($expr, $inner);
        },
        ;

    loop {
        unless $ast ~~ malList {
            return eval-ast($ast, $env);
        }

        my ($op, @args) = $ast.value.list;
        if $op ~~ malSymbol && %special{$op.value} -> $handler {
            return $handler(|@args);
            CATCH {
                die RuntimeError.new($op.value ~ ': ' ~ $_);
            }
        }
        elsif $op ~~ malSymbol && %special-tailrec{$op.value} -> $handler {
            ($ast, $env) = $handler(|@args);
            next; # TCO - continue the loop
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
                ($ast, $env) = ($op.value, $inner);
                next; # TCO - continue the loop
            }
            die RuntimeError(pr-str($op, True) ~ " is not applicable");
        }
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
