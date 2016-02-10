unit module ReadLine;

use v6;
use Linenoise;

# Load up the .mal-history file
my $history-path = %*ENV<MAL_HISTORY> // %*ENV<HOME> ~ '/.mal-history';
linenoiseHistoryLoad($history-path);

# Our single export. Returns a string, or undef on EOF.
sub read-line(Str $prompt) is export {
    my $line = linenoise($prompt);
    if (defined $line) {
        linenoiseHistoryAdd($line);
        linenoiseHistorySave($history-path);
    }
    return $line;
}
