package MAL::Reader;
use 5.20.0;
use warnings;

use Function::Parameters qw( :strict );
use MAL::Object;

package Reader {
    use Moo;
    use Types::Standard qw( ArrayRef Str );

    has input => (
        is => 'ro',
        isa => Str,
    );

    method peek { return $self->_tokens->[0]->{value}; }
    method next { shift(@{ $self->_tokens }); }
    method empty { return scalar @{ $self->_tokens } == 0; }

    method debug($msg) {
#        use Data::Dumper::Concise; print STDERR $msg, Dumper($self->_tokens);
    }

    method token_error($msg, $pos = undef) {
        $pos //= $self->_tokens->[0]->{position};
        my $inset = $pos + 1;
        my $input = $self->input;
        die sprintf("Error: $msg\n$input\n%${inset}s\n", '^');
    }

    has _tokens => (
        is => 'lazy',
        isa => ArrayRef,
        builder => 1,
    );

    method _build__tokens {
        my $re = qr(
            [\s,]*
            (   ~@
            |   [\[\]{}()'`~^@]
            |   "(?:\\.|[^\\"])*"
            |   ;.*
            |   [^\s\[\]{}('"`,;)]*
            )
        )x;
        my @tokens;
        my $input = $self->input;
        while ($input =~ m/$re/g) {
            if (length($1) == 0) {
                if (pos($input) != length($input)) {
                    if (substr($input, pos($input), 1) eq '"') {
                        die "Expected '\"', got EOF\n";
                    }
                    $self->token_error('unexpected input', pos($input));
                }
                else {
                    next;
                }
            }
            my $token = { value => $1, position => $-[0] };
            push(@tokens, $token);
        }

        return [ grep { ! /^[ ,]*$/ } @tokens ];
    }
};

fun _read_atom($reader) {
    $reader->debug('_read_atom');
    my $value = $reader->peek;
    $reader->next;

    if ($value =~ /^"(.*)"$/) {
        my $str = $1;
        $str =~ s/\\"/"/g;
        $str =~ s/\\n/\n/g;
        return MAL::Object->string($str);
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
    $reader->debug('_read_list');
    if ($reader->empty) {
        die "Expected ')', got EOF\n";
    }
    if ($reader->peek eq ')') {
        $reader->next;
        return MAL::Object->nil;
    }
    else {
        my $car = _read_form($reader);
        my $cdr = _read_list($reader);
        return MAL::Object->pair($car, $cdr);
    }
}

fun _read_form($reader) {
    $reader->debug('_read_form');
    if ($reader->peek eq '(') {
        $reader->next; # consume the left paren
        return _read_list($reader);
    }
    else {
        my $atom = _read_atom($reader);
#        use Data::Dumper::Concise; print STDERR Dumper($atom);
        return $atom;
    }
}

method read_str($class: $input) {
    my $reader = Reader->new(input => $input);
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
