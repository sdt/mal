package MAL::Object::List;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Sequence );
use Function::Parameters qw( :strict );

sub ldelim { '(' }
sub rdelim { ')' }

method car { return $self->first }
method cdr { return $self->rest  }

method to_list { return $self }

method conj(@items) { # does preserve meta
    my $clone = $self->clone;
    unshift(@{ $clone->{value} }, $_) for @items;
    return $clone;
}

1;
