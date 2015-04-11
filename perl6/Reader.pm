module Reader;

use v6;

use Types;

grammar MALGrammar {
    rule TOP { <ws>? <form>? <comment>? }

    rule form { <seq> | <atom> }

    rule seq { <seq-begin> <form>* <seq-end> }

    token comment   { \; .* $$ }
    token ws        { <[ \s \, ]>* }
    token seq-begin { < [ ( { > }
    token seq-end   { < ] ) } > | $ }

    token atom { <integer> }

    token integer { < + - >? \d+ }
}

#`<<
grammar MALGrammar {
    rule TOP { <ws>? <form>? <comment>? | { self.panic($/, "What?") }}

    rule form { <s-expr> | <vector> | <hash> | <atom> | { self.panic($/, "Huh?")} }

    rule s-expr { '(' <form>* ')' | { die "expected ')', got EOF" } }
    rule vector { '[' <form>* ']' | { die "expected ']', got EOF" } }
    rule hash   { '{' <form>* '}' }

    token atom { <macro> | <string> | <integer> | <word> }

    rule macro   { <macro-prefix> <form> }
    token macro-prefix { < ~@ ' ` ~ @ > }

#    rule with-meta { \^ <form> <form> }

    token word    { <[ \S ] - [ \[ \] \{ \} \( \) \' \" \` \, \; \) ]>+ }
    token integer { < + - >? \d+ }
    token string  { \" [ \\ . | <-["]> ]* \" }

    token comment { \; .* $$ }
    token ws      { <[ \s \, ]>* }

    method panic($/, $msg) {
        my $c = $/.CURSOR;
        my $pos := $c.pos;
        die "$msg found at pos $pos";
    }
}
>>

class MALGrammar::Actions {
    method string($/) {
        make Value.new(type => String, value => $/.Str);
    }

    method integer($/) {
        make Value.new(type => Integer, value => $/.Int);
    }

    method word($/) {
        my $value = $/.Str;
        my $type = $value.substr(0, 1) eq ':' ?? Keyword !! Symbol;
        make Value.new(type => $type, value => $value);
    }

    method macro($/) {
        my %macro = q/@/  => 'deref',
                    q/`/  => 'quasiquote',
                    q/'/  => 'quote',
                    q/~@/ => 'splice-unquote',
                    q/~/  => 'unquote',
                    ;

        my $prefix = $<macro-prefix>.Str;
        make Value.new(type => List, value => (
                Value.new(type => Symbol, value => %macro{$prefix}),
                $<form>.ast
            ));
    }

    method atom($/) {
        make $/.values[0].ast;
    }

    method s-expr($/) {
        make Value.new(type => List, value => $<form>.map({$_.ast}));
    }

    method hash($/) {
        make Value.new(type => HashMap, value => $<form>».ast);
    }

    method vector($/) {
        make Value.new(type => Vector, value => $<form>».ast);
    }

    method seq($/) {
        my %seq-info =
            '(' => %( end => ')', type => List    ),
            '[' => %( end => ']', type => Vector  ),
            '{' => %( end => '}', type => HashMap ),
            ;

        my $end = $<seq-end>.Str;
        $end = 'EOF' if $end eq '';

        my $info = %seq-info{ $<seq-begin>.Str };
        if $end ne $info<end> {
            die "Expected $info<end>, got $end";
        }
        make Value.new(type => $info<type>, value => $<form>.map({$_.ast}));
    }

    method form($/) {
        make $/.values[0].ast;
    }

    method TOP($/) {
        make $<form>.ast;
    }
}

sub read-str(Str $input) is export {
    my $match = MALGrammar.parse($input, :actions(MALGrammar::Actions.new));
    if $match ~~ Match {
        return $match.ast;
    }
}
