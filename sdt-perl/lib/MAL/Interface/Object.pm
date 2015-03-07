package MAL::Interface::Object;
use 5.20.0;
use warnings;

use Moo::Role;
use Function::Parameters qw( :strict );

requires qw( to_string );

method in_parens(@args) {
    return '(', join(' ', @args), ')';
}

method is_nil {
    return $self->isa('MAL::Object::Nil');
}

1;
