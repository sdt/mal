#!/usr/bin/env perl6

use v6;
use Core;
use Printer;
use Reader;
use ReadLine;
use Types;

my @library =
    '(def! not (fn* (a) (if a false true)))',
    '(def! load-file (fn* (f) (eval (read-string (str "(do " (slurp f) ")")))))',
    ;

# This gets called when there is no argv
multi sub MAIN() {
    my $repl-env = setup-env([]);

    while defined (my $input = read-line('user> ')) {
        say rep($input, $repl-env);
    }
}

# This gets called when there is an argv, and the first arg is a file
multi sub MAIN(Str $filename, *@args) {
    my $repl-env = setup-env(@args);

    rep("(load-file \"$filename\")", $repl-env);
}

sub setup-env(@argv) {
    my $env = malEnv.new;
    install-core($env, :eval({ malEVAL($_, $env) }));

    for @library { rep($_, $env) }

    # Install argv
    $env.set('*ARGV*', malList.new(@argv.map: { malString.new($_) }));

    return $env;
}

sub rep(Str $input, $env) {
    return malPRINT(malEVAL(malREAD($input), $env));

    CATCH {
        when Reader::EmptyInput {
            # nothing
        }
        default {
            say %*ENV<MAL_DEBUG>:exists ?? $_ !! $_.message;
        }
    }
}

sub malREAD(Str $input) {
    return read-str($input);
}

sub malEVAL(malValue $ast is copy, malEnv $env is copy) {

    my %special =
        'def!' => sub (malSymbol $sym, malValue $def) {
            $env.set($sym.value, malEVAL($def, $env))
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
                malEVAL($value, $env);
            }
            return (@values[$n-1], $env);
        },
        'if' => sub (malValue $cond, malValue $then, malValue $else = malNil) {
            my $ast = is-true(malEVAL($cond, $env)) ?? $then !! $else;
            return ($ast, $env);
        },
        'let*' => sub (malSequence $bindings, malValue $expr) {
            my $inner = malEnv.new(:outer($env));
            for $bindings.value.list -> malSymbol $symbol, malValue $value {
                $inner.set($symbol.value, malEVAL($value, $inner));
            }
            return ($expr, $inner);
        },
        ;

    loop {
        unless ($ast ~~ malList) && ($ast.value.elems > 0) {
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
            die RuntimeError.new(pr-str($op, True) ~ " is not applicable");
        }
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