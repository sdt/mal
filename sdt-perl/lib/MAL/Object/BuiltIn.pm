package MAL::Object::BuiltIn;
use 5.20.0;
use warnings;

use parent qw( MAL::Object::Applyable );

use Function::Parameters qw( :strict );

method new($class: $name, $subref, $return_type) {
    bless {
        name => $name,
        return_type => $return_type,
        subref => $subref,
    }, $class;
}

method clone {
    my $clone = { %$self };
    return bless $clone, ref $self;
};

method apply($args) {
    my $type = $self->{return_type};
    if ($type) {
        # Box the return value
        return MAL::Object->$type(
            $self->{subref}->($args->items) # may be scalar or array
        );
    }
    else {
        # Return the value directly
        return $self->{subref}->($args->items);
    }
}

method to_string {
    return $self->{name};
}

1;
