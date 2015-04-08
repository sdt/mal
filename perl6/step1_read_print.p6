#!/usr/bin/env perl6

use v6;
use Reader;
use ReadLine;

sub MAIN() {
    while defined (my $input = read-line('user> ')) {
        say rep($input);
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
    return $ast;
}
