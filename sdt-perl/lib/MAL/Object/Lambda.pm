package MAL::Object::Lambda;
use 5.20.0;
use warnings;

use parent qw( MAL::Object );

use Function::Parameters qw( :strict );
use MAL::Environment;

method new($class: $parameters, $expression, $env, $eval) {
    bless {
        expression => $expression,
        parameters => [ map { $_->value } $parameters->items ],
        env => $env,
        eval => $eval, # this is EVAL, kind of a hack passing it in
    }, $class;
}

fun make_hash($keys, $values) {
    my @k = @$keys;
    my @v = @$values;
    my %hash;
    while (@k && @v) {
        $hash{shift @k} = shift @v;
    }
    return %hash;
}

method apply($args) {
    my $inner = MAL::Environment->new(
        outer => $self->{env},
        data  => { make_hash($self->{parameters}, [ $args->items ]) },
    );
    return $self->{eval}->($self->{expression}, $inner);
}

method to_string {
    return '#function';
}

1;
