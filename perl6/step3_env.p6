#!/usr/bin/env perl6

use v6;
use Environment;
use Printer;
use Reader;
use ReadLine;
use Types;

sub MAIN() {
    my $repl-env = Env.new;
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

sub EVAL($ast, $env) {
    given ($ast.type) {
        when List {
            my ($op, @args) = eval-ast($ast, $env).value.list;
            return $op(|@args);
        }
        default {
            return eval-ast($ast, $env);
        }
    }
}

sub PRINT($ast) {
    return pr-str($ast, True);
}

sub eval-ast($ast, $env) {
    my $type = $ast.type;
    given ($type) {
        when Symbol {
            return $env.get($ast.value);
        }
        when HashMap {
            my %value = $ast.value.pairs.map(
                { $_.key => EVAL($_.value, $env) });
            return Value.new(:$type, :%value);
        }
        when List | Vector {
            my @value = $ast.value.map({ EVAL($_, $env) });
            return Value.new(:$type, :@value);
        }
        default {
            return $ast;
        }
    }
}

sub wrap-int-op($native-func) {
    return sub (Value $a, Value $b) {
        my $value = $native-func($a.value, $b.value);
        return Value.new(:type(Integer), :$value);
    }
}
