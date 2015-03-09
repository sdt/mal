package MAL::Object::List;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Sequence );
use Function::Parameters qw( :strict );

sub ldelim { '(' }
sub rdelim { ')' }

method car { return $self->item(0) }
method cdr {
    my @items = $self->items;
    shift @items;
    return __PACKAGE__->new(@items);
}

1;
