package MAL::Object::Vector;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Sequence );
use Function::Parameters qw( :strict );

sub ldelim { '[' }
sub rdelim { ']' }

method to_list { return MAL::Object::List->new($self->items); }

1;
