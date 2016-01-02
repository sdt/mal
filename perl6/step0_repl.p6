#!/usr/bin/env perl6

use v6;
use ReadLine;

sub MAIN() {
    while defined (my $input = read-line('user> ')) {
        say rep($input);
    }
}

sub rep(Str $input) {
    malPRINT(malEVAL(malREAD($input)));
}

sub malREAD(Str $input) {
    return $input;
}

sub malEVAL($ast) {
    return $ast;
}

sub malPRINT($ast) {
    return $ast;
}
