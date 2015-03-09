package MAL::Tokeniser;
use 5.20.0;
use warnings;

use Moo;
use Function::Parameters qw( :strict );
use Types::Standard qw( ArrayRef Str );

has input => (
    is => 'ro',
    isa => Str,
);

method peek { return $self->_tokens->[0]->{value}; }
method empty { return scalar @{ $self->_tokens } == 0; }
method next {
    my $token = $self->peek;
    shift(@{ $self->_tokens });
    return $token;
}

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

    @tokens = grep { $_->{value} !~ /^;/ } @tokens; # strip comments
    return \@tokens;
}

1;
