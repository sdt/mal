module Reader;

use v6;

use Types;

class ParseError is malException { }
class EmptyInput is malException {
    method new() { self.bless(:reason('Empty input')) }
}

grammar MALGrammar {
    rule TOP            { ^ <form>? $ }

    rule form           { <seq> | <atom> }

    rule seq            { <seq-begin> <ws>? <form>* %<ws> <ws>? <seq-end> }

    token seq-begin     { < [ ( { > }
    token seq-end       { < ] ) } > | $ }

    token atom          { <macro> | <meta> | <string> | <word> }

    rule macro          { <macro-prefix> <form> }
    token macro-prefix  { < ~@ ' ` ~ @ > }

    rule meta           { \^ <form> <form> }

    token string        { <string-begin> <string-body> <string-end> }
    token string-begin  { \" }
    token string-body   { [ \\ . | <-["]> ]* }
    token string-end    { \" | $ }


    token word          { <[ \S ] - [ \[ \] \{ \} \( \) \' \" \` \, \; \) ]>+ }

    token comment       { \; .*? $$ }
    token ws            { <[ \s \, ]>* | <comment>* }
}

class MALGrammar::Actions {
    method TOP($/) {
        if $<form> ~~ Nil {
            die EmptyInput.new;
        };
        make $<form>.ast;
    }

    method form($/) {
        make $/.values[0].ast;
    }

    method seq($/) {
        my %seq-info =
            '(' => %( end => ')', type => malList   ),
            '[' => %( end => ']', type => malVector ),
            '{' => %( end => '}', type => malHash   ),
            ;

        my $end = $<seq-end>.Str;
        $end = 'EOF' if $end eq '';

        my $info = %seq-info{ $<seq-begin>.Str };
        if $end ne $info<end> {
            die ParseError.new("Expected $info<end>, got $end");
        }
        my $type = $info<type>;
        my @value = $<form>.map({$_.ast}).list;
        if $type ~~ malHash {
            return make make-hash(@value);
        }
        make $type.new(@value);
    }

    method atom($/) {
        make $/.values[0].ast;
    }

    method macro($/) {
        my %macro = q/@/  => 'deref',
                    q/`/  => 'quasiquote',
                    q/'/  => 'quote',
                    q/~@/ => 'splice-unquote',
                    q/~/  => 'unquote',
                    ;

        my $prefix = $<macro-prefix>.Str;
        make make-form(%macro{$prefix}, $<form>.ast);
    }

    method meta($/) {
        my ($meta, $object) = $<form>[0, 1];
        make make-form('with-meta', $object.ast, $meta.ast);
    }

    method string($/) {
        if $<string-end>.Str eq '' {
            die ParseError.new("Expected \", got EOF");
        }
        make malString.new(unescape-string($/.Str));
    }

    method word($/) {
        try {
            return make malInteger.new($/.Int);
        }
        my $value = $/.Str;
        my %constants = true  => malTrue,
                        false => malFalse,
                        nil   => malNil;
        given $value {
            when %constants {
                make %constants{$value};
            }
            when .substr(0, 1) eq ':' {
                make malKeyword.new($value);
            }
            default {
                make malSymbol.new($value);
            }
        }
    }
}

sub read-str(Str $input) is export {
    my $match = MALGrammar.parse($input, :actions(MALGrammar::Actions.new));
    if $match ~~ Match {
        return $match.ast;
    }
    die ParseError.new('Syntax error');
}
