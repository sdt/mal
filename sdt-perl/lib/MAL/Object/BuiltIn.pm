package MAL::Object::BuiltIn;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class: $name, $subref, $return_type) {
    bless {
        name => $name,
        return_type => $return_type,
        subref => $subref,
    }, $class;
}

method apply($args) {
    my @argvals = map { $_->value } $args->items;
    my $ret = $self->{subref}->(@argvals);
    return $self->{return_type}->new($ret);
}

method to_string {
    return $self->{name};
}

1;
