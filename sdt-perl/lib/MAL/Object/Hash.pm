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
    bless { value => \%hash, }, $class;
}

method clone {
    my $clone = {
        value => { %{ $self->{value } } }, # new hash, same kv's
    };
    $clone->{meta} = $self->{meta} if exists $self->{meta};
    return bless $clone, ref $self;
};

method to_string($readable = 0) {
    my $hash = $self->{value};
    return '{' .
        join(' ',
            map { $_ => $hash->{$_}->to_string($readable) } $self->get_keys
        ) .
    '}';
}

method get($key) {
    return $self->{value}->{make_key($key)} // MAL::Object->nil;
}

method contains($key) {
    return exists $self->{value}->{make_key($key)};
}

method get_keys {
    return sort keys %{$self->{value}};
}

method get_vals {
    return map { $self->{value}->{$_} } $self->get_keys;
}

method map_values($f) {
    return $self->_with_clone(fun($hash) {
        for my $key (keys %$hash) {
            $hash->{$key} = $f->($hash->{$key});
        }
    });
}

method assoc(@pairs) {
    return $self->_with_clone(fun($hash) {
        while (@pairs) {
            my ($key, $value) = splice(@pairs, 0, 2);
            $hash->{make_key($key)} = $value;
        }
    });
}

method dissoc(@keys) {
    return $self->_with_clone(fun($hash) {
        for my $key (@keys) {
            delete $hash->{make_key($key)};
        }
    });
}

method _with_clone($f) {
    my $clone = $self->clone;
    $f->($clone->{value});
    return $clone;
}

1;
