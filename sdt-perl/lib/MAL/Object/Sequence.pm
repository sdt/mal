package MAL::Object::Sequence;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class: @items) {
    bless [ @items ], $class;
}

method is_sequence {
    return 1;
}

method items {
    return @$self;
}

method item($index) {
    return $self->[$index];
}

method first {
    return $self->[0];
}

method rest {
    my @items = @$self;
    shift @items;
    return bless [ @items ], ref $self;
}

method length {
    return scalar(@$self);
}

method to_string($readable = 0) {
    return
        $self->ldelim .
        join(' ', map { $_->to_string($readable) } $self->items) .
        $self->rdelim;
}

method map_items($f) {
    my @mapped = map { $f->($_) } $self->items;
    my $class = ref $self;
    return $class->new(@mapped);
}

method equal($rhs) {
    return unless $rhs->is_sequence;
    return unless $self->length == $rhs->length;
    for my $i (0 .. $self->length-1) {
        return unless $self->item($i)->equal($rhs->item($i));
    }
    return 1;
}

1;
