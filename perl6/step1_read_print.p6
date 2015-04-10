#!/usr/bin/env perl6

use v6;
use Printer;
use Reader;
use ReadLine;

sub MAIN() {
    while defined (my $input = read-line('user> ')) {
        say rep($input);

        CATCH {
            default {
                say $_.message;
            }
        }
    }
}

sub rep(Str $input) {
    PRINT(EVAL(READ($input)));
}

sub READ(Str $input) {
    return read-str($input);
}

sub EVAL($ast) {
    return $ast;
}

sub PRINT($ast) {
    return pr-str($ast);
}
