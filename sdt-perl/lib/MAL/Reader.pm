package MAL::Reader;
use 5.20.0;
use warnings;

use Function::Parameters qw( :strict );
use MAL::Object;
use MAL::Tokeniser;

fun _read_atom($reader) {
    $reader->debug('_read_atom');
    my $value = $reader->next;

    my %quote = (
        q(')  => 'quote',
        q(`)  => 'quasiquote',
        q(~)  => 'unquote',
        q(@)  => 'deref',
        q(~@) => 'splice-unquote',
    );

    for my $quotesym (keys %quote) {
        if ($value eq $quotesym) {
            my $quotetype = $quote{$quotesym};
            if ($reader->empty) {
                die "Expecting form to $quotetype, got EOF\n";
            }
            return MAL::Object->list(
                MAL::Object->symbol($quotetype),
                _read_form($reader),
            );
        }
    }

    if ($value eq '^') {
        if ($reader->empty) {
            die "Expecting first form to with-meta, got EOF\n";
        }
        my $first = _read_form($reader);
        if ($reader->empty) {
            die "Expecting second form to with-meta, got EOF\n";
        }
        my $second = _read_form($reader);
        return MAL::Object->list(
                MAL::Object->symbol('with-meta'),
                $second,
                $first,
            );
    }

    if ($value =~ /^"(.*)"$/) {
        my $str = $1;
        $str =~ s/\\"/"/g;
        $str =~ s/\\n/\n/g;
        return MAL::Object->string($str);
    }

    if ($value =~ /^:/) {
        return MAL::Object->keyword($value);
    }

    if ($value =~ /^\d+$/) {
        return MAL::Object->integer($value);
    }

    if ($value =~ /^\d/) {
        $reader->token_error('syntax error');
    }

    for my $constant (qw( true false nil )) {
        if ($value eq $constant) {
            return MAL::Object->$constant;
        }
    }
    return MAL::Object->symbol($value);
}

fun _read_list($reader) {
    my %close = (
        '(' => ')',
        '[' => ']',
        '{' => '}',
    );
    my $terminator = $close{$reader->next};
    $reader->debug("_read_list:$terminator");
    my @items;
    while (1) {
        if ($reader->empty) {
            die "Expected '$terminator', got EOF\n";
        }
        my $value = $reader->peek;
        if ($value eq $terminator) {
            $reader->next;
            return @items;
        }
        push(@items, _read_form($reader));
    }
    return @items;
}

fun _read_form($reader) {
    $reader->debug('_read_form');
    if ($reader->peek eq '(') {
        return MAL::Object->list(_read_list($reader));
    }
    elsif ($reader->peek eq '[') {
        return MAL::Object->vector(_read_list($reader));
    }
    elsif ($reader->peek eq '{') {
        return MAL::Object->hash(_read_list($reader));
    }
    else {
        my $atom = _read_atom($reader);
#        use Data::Dumper::Concise; print STDERR Dumper($atom);
        return $atom;
    }
}

method read_str($class: $input) {
    my $reader = MAL::Tokeniser->new(input => $input);
    if ($reader->empty) {
        return MAL::Object->nil;
    }
    my $ast = _read_form($reader);
    if (!$reader->empty) {
        $reader->token_error('unexpected extra input');
    }
    return $ast;
}


1;
__END__

=head1 NAME

MAL::Reader

=cut
