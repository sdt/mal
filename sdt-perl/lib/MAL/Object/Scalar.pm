package MAL::Object::Scalar;
use 5.20.0;
use warnings;

use Function::Parameters qw( :strict );

use parent qw( MAL::Object );

method new($class: $value) {
    bless \$value, $class;
}

method value {
    return $$self;
}

method to_string {
    return "$$self";
};

1;
