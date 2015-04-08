module Reader;

use v6;

use Types;

grammar MALGrammar {
    token TOP { <form>? <comment>? }

    rule form { <s-expr> | <vector> | <hash> | <atom> }

    rule s-expr { '(' <form>* ')' }
    rule vector { '[' <form>* ']' }
    rule hash   { '{' <form>* '}' }

    token atom { <macro> | <string> | <word> }

    rule macro   { <macro-prefix> <form> }
    token macro-prefix { < ~@ [ ' ` ~ ^ @ > }

    token word   { <[ \S ] - [ \[ \] \{ \} \( \) \' \" \` \, \; \) ]>+ }
    token string {
        \"
        [
        | \\ .      # Escaped chars are okay
        | <-["]>    # And anything except a "
        ]*
        \"
    }

    token comment { \; .* $$ }
    token ws { <[ \s \, ]>* }
}

class MALGrammar::Actions {
    method string($/) {
        make Value.new(type => String, value => $/.Str);
    }

    method word($/) {
        my $value = $/.Str;
        given $value {
#            when / ^ <[ \+ \- ]>? \d+ $ / {
#                make Value.new(type => Integer, value => 0 + $/.Str);
#            }

#            when / ^ \: / {
#                make Value.new(type => Keyword, value => $value);
#            }

            default {
                make Value.new(type => Symbol, value => $/.Str);
            }
        }
    }

    method macro($/) {
        say "MACRO:\n", $/;
    }

    method atom($/) {
        make $/.values[0].ast;
    }

    method s-expr($/) {
        make Value.new(type => List, value => $<form>».ast);
    }

    method hash($/) {
        make Value.new(type => HashMap, value => $<form>».ast);
    }

    method vector($/) {
        make Value.new(type => Vector, value => $<form>».ast);
    }

    method form($/) {
        make $/.values[0].ast;
    }

    method TOP($/) {
        make $<form>».ast;
    }
}

sub read-str(Str $input) is export {
    my $match = MALGrammar.parse($input, :actions(MALGrammar::Actions.new));
    say "TOP:\n", $match.ast;
    say "NO MATCH" unless $match;
    return $input;
}
