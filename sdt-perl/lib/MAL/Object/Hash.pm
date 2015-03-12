package MAL::Object::Hash;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

fun make_key($key) {
    return $key->to_string(1);
}

method new($class: @items) {
    die 'Odd number of elements in hash' if @items % 2;
    my %hash;
    while (@items) {
        my $key = shift @items;
        die "Keys to hash must be keywords or strings"
            unless $key->is_string || $key->is_keyword;
        my $value = shift @items;
        $hash{make_key($key)} = $value;
    }
    bless { %hash }, $class;
}

method to_string($readable = 0) {
    return '{' .
        join(' ',
            map { $_ => $self->{$_}->to_string($readable) } sort keys %$self
        ) .
    '}';
}

method get($key) {
    return $self->{make_key($key)} // MAL::Object->nil;
}

method contains($key) {
    return exists $self->{make_key($key)};
}

method get_keys {
    return sort keys %$self
}

method get_vals {
    return map { $self->{$_} } $self->get_keys;
}

method map_values($f) {
    my %hash;
    for my $key (keys %$self) {
        $hash{$key} = $f->($self->{$key});
    }
    return bless { %hash }, ref $self;
}

method assoc(@pairs) {
    my %hash = %$self;
    while (@pairs) {
        my ($key, $value) = splice(@pairs, 0, 2);
        $hash{make_key($key)} = $value;
    }
    return bless { %hash }, ref $self;
}

method dissoc(@keys) {
    my %hash = %$self;
    delete @hash{ map { make_key($_) } @keys };
    return bless { %hash }, ref $self;
}


1;
