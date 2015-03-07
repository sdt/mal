package MAL::Object::String;
use 5.20.0;
use warnings;

use Moo;
with qw( MAL::Interface::Object );

use Function::Parameters qw( :strict );
use Types::Standard qw( Str );

has value => (
    is => 'ro',
    isa => Str,
);

method to_string {
    return '"' . $self->value . '"';
}

1;
