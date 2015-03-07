package MAL::Object::Integer;
use 5.20.0;
use warnings;

use Moo;
with qw( MAL::Interface::Object );

use Function::Parameters qw( :strict );
use Types::Standard qw( Int );

has value => (
    is => 'ro',
    isa => Int,
);

method to_string {
    return $self->value;
}

1;
