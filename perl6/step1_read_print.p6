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
                say %*ENV<MAL_DEBUG>:exists ?? $_ !! $_.message;
            }
            default {
                say 'Unexpected exception: ', $_.gist;
            }
        }
    }
}

sub rep(Str $input) {
    malPRINT(malEVAL(malREAD($input)));
}

sub malREAD(Str $input) {
    return read-str($input);
}

sub malEVAL($ast) {
    return $ast;
}

sub malPRINT($ast) {
    return pr-str($ast, True);
}
