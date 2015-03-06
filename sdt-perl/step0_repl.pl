#!/usr/bin/env perl

use 5.20.1;
use warnings;

use Function::Parameters qw( :strict );
use Term::ReadLine;

exit main(@ARGV) ? 0 : 1;

#-------------------------------------------------------------------------------

fun main(@argv) {
    my $term = Term::ReadLine->new('mal');
    my $prompt = 'user>';
    while (defined(my $input = $term->readline($prompt))) {
        chomp($input);
        say rep($input);
        $term->addhistory($input) if $input =~ /\S/;
    }
    return 0;
}

fun rep($arg) {
    return PRINT(EVAL(READ($arg)));
}

fun READ($arg) {
    return $arg;
}

fun EVAL($arg) {
    return $arg;
}

fun PRINT($arg) {
    return $arg;
}

