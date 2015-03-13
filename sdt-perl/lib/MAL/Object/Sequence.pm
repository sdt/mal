package MAL::Object::Sequence;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class: @items) {
    die "Cannot create $class items directly\n" if $class eq __PACKAGE__;
    bless {
        value => \@items,
    }, $class;
}

method clone {
    my $clone = {
        value => [ @{ $self->{value } } ], # new array, same values
    };
    $clone->{meta} = $self->{meta} if exists $self->{meta};
    return bless $clone, ref $self;
};

method is_sequence {
    return 1;
}

method items {
    return @{$self->{value}};
}

method item($index) {
    return $self->{value}->[$index];
}

method first {
    return $self->{value}->[0];
}

method rest {
    my @items = $self->items;
    shift @items;
    my $class = ref $self;
    return $class->new(@items);
}

method length {
    return scalar($self->items);
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
