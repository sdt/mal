module ReadLine;

use v6;

# Import some native functions from GNU readline.
use NativeCall;
sub readline(Str $prompt) returns Str
    is native('libreadline') { ... }
sub add_history(Str $line)
    is native('libreadline') { ... }
sub append_history(Int $lineno, Str $filename) returns Int
    is native('libreadline') { ... }
sub read_history(Str $filename) returns Int
    is native('libreadline') { ... }

# Load up the .mal-history file
my $history-path = %*ENV<MAL_HISTORY> // %*ENV<HOME> ~ '/.mal-history';
if ($history-path.IO ~~ :e) {
    read_history($history-path);
}
else {
    open($history-path, :w).close(); # Not sure why this is necessary
}

# Our single export. Returns a string, or undef on EOF.
sub read-line(Str $prompt) is export {
    my $line = readline($prompt);
    if (defined $line) {
        add_history($line);
        append_history(1, $history-path);
    }
    return $line;
}
