#!/usr/bin/env perl6

use v6;
use Printer;
use Reader;
use ReadLine;
use Types;

sub MAIN() {
    my %repl-env = '+' => wrap-int-op(&[+]),
                   '-' => wrap-int-op(&[-]),
                   '*' => wrap-int-op(&[*]),
                   '/' => wrap-int-op(&[/]),
                   ;

    while defined (my $input = read-line('user> ')) {
        say rep($input, %repl-env);

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

sub rep(Str $input, %env) {
    PRINT(EVAL(READ($input), %env));
}

sub READ(Str $input) {
    return read-str($input);
}

sub EVAL($ast, %env) {
    given ($ast) {
        when malList {
            my ($op, @args) = eval-ast($ast, %env).value.list;
            return $op(|@args);
        }
        default {
            return eval-ast($ast, %env);
        }
    }
}

sub PRINT($ast) {
    return pr-str($ast, True);
}

sub eval-ast($ast, %env) {
    given ($ast) {
        when malSymbol {
            return %env{$ast.value};
        }
        when malHash {
            my %value = $ast.value.pairs.map(
                { $_.key => EVAL($_.value, %env) });
            return malHash.new(%value);
        }
        when malList {
            my @value = $ast.value.map({ EVAL($_, %env) });
            return malList.new(@value);
        }
        when malVector {
            my @value = $ast.value.map({ EVAL($_, %env) });
            return malVector.new(@value);
        }
        default {
            return $ast;
        }
    }
}

sub wrap-int-op($native-func) {
    return sub (malInteger $a, malInteger $b) {
        my $value = $native-func($a.value, $b.value);
        return malInteger.new($value.Int);
    }
}
