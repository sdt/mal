package MAL::Object::Constant;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );

method new($class:) {
    my $name = lc($class);
    $name =~ s/^.*:://;
    bless \$name, $class;
}

method to_string {
    return $$self;
}

method equal($rhs) {
    return $self->same_type($rhs); # there is no value, so type check is enough
}

1;
