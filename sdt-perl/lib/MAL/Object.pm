package MAL::Object;
use 5.20.0;
use warnings;

use Function::Parameters qw( :strict );
use Module::Load;

my @types = qw( Integer Nil Pair String Symbol );

load "MAL::Object::$_" for @types;

method integer($class: $value) {
    return MAL::Object::Integer->new(value => $value);
}

method nil($class:) {
    state $nil = MAL::Object::Nil->new;
    return $nil;
}

method pair($class: $car, $cdr) {
    return MAL::Object::Pair->new(car => $car, cdr => $cdr);
}

method string($class: $value) {
    return MAL::Object::String->new(value => $value);
}

method symbol($class: $value) {
    return MAL::Object::Symbol->new(value => $value);
}

1;
