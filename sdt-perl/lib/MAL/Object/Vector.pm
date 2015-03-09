package MAL::Object::Vector;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Sequence );

sub ldelim { '[' }
sub rdelim { ']' }

1;
