package MAL::Object::Hash;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class: @items) {
    die 'Odd number of elements in hash' if @items % 2;
    my %hash;
    while (@items) {
        my $key = shift @items;
        die "Keys to hash must be keywords or strings"
            unless $key->is_string || $key->is_keyword;
        my $value = shift @items;
        $hash{$key->to_string} = $value;
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
    return $self->{$key};
}

method map_values($f) {
    my %hash;
    for my $key (keys %$self) {
        $hash{$key} = $f->($self->{$key});
    }
    return bless { %hash }, ref $self;
}

1;
