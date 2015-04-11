module Reader;

use v6;

use Types;

grammar MALGrammar {
    rule TOP            { <ws>? <form>? <comment>? }

    rule form           { <seq> | <atom> }

    rule seq            { <seq-begin> <form>* <seq-end> }

    token seq-begin     { < [ ( { > }
    token seq-end       { < ] ) } > | $ }

    token atom          { <integer> | <macro> | <string> | <word> }

    token integer       { < + - >? \d+ }

    rule macro          { <macro-prefix> <form> }
    token macro-prefix  { < ~@ ' ` ~ @ > }

    token string        { <string-begin> <string-body> <string-end> }
    token string-begin  { \" }
    token string-body   { [ \\ . | <-["]> ]* }
    token string-end    { \" | $ }

#    rule with-meta { \^ <form> <form> }

    token word          { <[ \S ] - [ \[ \] \{ \} \( \) \' \" \` \, \; \) ]>+ }

    token comment       { \; .* $$ }
    token ws            { <[ \s \, ]>* }
}

class MALGrammar::Actions {
    method TOP($/) {
        make $<form>.ast;
    }

    method form($/) {
        make $/.values[0].ast;
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

    method atom($/) {
        make $/.values[0].ast;
    }

    method integer($/) {
        make Value.new(type => Integer, value => $/.Int);
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

    method string($/) {
        if $<string-end>.Str eq '' {
            die "Expected \", got EOF";
        }
        make Value.new(type => String, value => $<string-body>.Str);
    }

    method word($/) {
        my $value = $/.Str;
        my $type = $value.substr(0, 1) eq ':' ?? Keyword !! Symbol;
        make Value.new(type => $type, value => $value);
    }
}

sub read-str(Str $input) is export {
    my $match = MALGrammar.parse($input, :actions(MALGrammar::Actions.new));
    if $match ~~ Match {
        return $match.ast;
    }
}
