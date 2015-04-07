#!/usr/bin/env perl6

use v6;
use NativeCall;

sub readline(Str $prompt) returns Str
    is native('libreadline') { ... }
sub add_history(Str $line)
    is native('libreadline') { ... }
sub append_history(Int $lineno, Str $filename) returns Int
    is native('libreadline') { ... }
sub read_history(Str $filename) returns Int
    is native('libreadline') { ... }

my $history = %*ENV<MAL_HISTORY> // %*ENV<HOME> ~ '/.mal-history';
if ($history.IO ~~ :e) {
    read_history($history);
}
else {
    open($history, :w).close(); # Not sure why this is necessary
}

while (defined (my $line = readline('user> '))) {
    say "-> $line";
    add_history($line);
    append_history(1, $history);
}
say 'Done';
