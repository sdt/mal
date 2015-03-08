package MAL::Environment;
use 5.20.0;
use warnings;

use Moo;
use Types::Standard qw( HashRef InstanceOf );

use Function::Parameters qw( :strict );

has data => (
    is => 'ro',
    isa => HashRef,
    default => sub { { } },
);

has outer => (
    is => 'ro',
    isa => InstanceOf['MAL::Environment'],
    predicate => 1,
);

method set($key, $value) {
    $self->data->{$key} = $value;
    return $value;
}

method find($key) {
    if (exists $self->data->{$key}) {
        return $self;
    }
    if ($self->has_outer) {
        return $self->outer->find($key);
    }
    return;
}

method get($key) {
    if (my $env = $self->find($key)) {
        return $env->data->{$key};
    }
    die "Undefined symbol $key\n";
}

1;
