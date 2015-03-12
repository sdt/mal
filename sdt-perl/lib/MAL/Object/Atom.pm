package MAL::Object::Atom;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Scalar );
use Function::Parameters qw( :strict );

method equal($rhs) {
    return $self->same_type($rhs) && $self->value->equal($rhs->value);
}

method reset($value) {
    $$self = $value;
    return $value;
}

method deref {
    return $self->value;
}

method to_string($readable = 0) {
    return '(atom ' . $self->value->to_string($readable) . ')';
}

1;
