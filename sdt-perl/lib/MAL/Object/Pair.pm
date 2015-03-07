package MAL::Object::Pair;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class: $car, $cdr) {
    bless [ $car, $cdr ], $class;
}

method car { $self->[0] }
method cdr { $self->[1] }

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
