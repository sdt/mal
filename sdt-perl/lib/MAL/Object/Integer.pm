package MAL::Object::Integer;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Scalar );
use Function::Parameters qw( :strict );

method equal($rhs) {
    return $self->same_type($rhs) && ($self->value == $rhs->value);
}

1;
