module Reader;

use v6;

grammar MALGrammar {
    token TOP { <form>? <comment>? }

    rule form { <s-expr> | <vector> | <hash> | <atom> }

    rule s-expr { '(' <list> ')' }
    rule vector { '[' <list> ']' }
    rule hash   { '{' <list> '}' }

    rule list { <form>* }

    token atom { <macro> | <string> | <comment> | <word> }

    rule macro   { <macro-prefix> <form> }
    token macro-prefix { '~@' | <[ \' \` \~ \^ \@ ]> }

    token comment { \; .* $$ }
    token word   { <[ \S ] - [ \[ \] \{ \} \( \) \' \" \` \, \; \) ]>+ }
    token string {
        \"
        [
        | \\ .
        | <-["\\]> # Anything not a " or \
#        | '\"'     # Or \", an escaped quotation mark
        ]*         # Any number of times
        \"
    }

    token ws { <[ \s \, ]>* }
}

sub read-str(Str $input) is export {
    my $match = MALGrammar.parse($input);
    say $match;
    return $input;
}
