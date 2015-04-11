#!/usr/bin/env perl6

use v6;
use Printer;
use Reader;
use ReadLine;

sub MAIN() {
    while defined (my $input = read-line('user> ')) {
        say rep($input);

        CATCH {
            when Reader::EmptyInput {
                # nothing
            }
            when Reader::ParseError {
                say $_.message;
            }
            default {
                say 'Unexpected exception: ', $_.gist;
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
    return pr-str($ast, True);
}
