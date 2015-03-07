package MAL::Object::Vector;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class: @items) {
    bless [ @items ], $class;
}

method items {
    return @$self;
}

method to_string {
    return '[' .  join(' ', map { $_->to_string } $self->items) . ']';
}

1;
