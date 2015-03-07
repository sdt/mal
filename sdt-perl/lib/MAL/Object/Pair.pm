package MAL::Object::Pair;
use 5.20.0;
use warnings;

use Moo;
with qw( MAL::Interface::Object );

use Function::Parameters qw( :strict );
use Types::Standard qw( ConsumerOf );

has [qw( car cdr )] => (
    is => 'rw',
    isa => ConsumerOf['MAL::Interface::Object'],
);

method to_string {
    return $self->in_parens(
        map { $_->to_string } $self->as_list
    );
}

method as_list {
    if ($self->cdr->is_nil) {
        return ( $self->car );
    }
    return ( $self->car, $self->cdr->as_list );
}

1;
