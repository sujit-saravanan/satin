# satin
A generic text preprocessor that was primarily designed to work with HTML. It provides basic text replacement capabilities with a simple, elegant syntax.

input:
```html
def macro_name(parameter1, parameter2, parameter3){
    <div class="$$parameter1"> this is $$parameter1 </div>
    <div class="$$parameter2"> this is $$parameter2  </div>
    <div class="$$parameter3"> this is $$parameter3  </div>
}

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
macro_name(a, b, c)

macro_name({there are spaces here}, {there|are|special|characters|here}, {You should use curly braces for inputs that aren't a single, continous word})
</body>
```

output:
```html
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>
<div class="a"> this is a </div>
<div class="b"> this is b  </div>
<div class="c"> this is c  </div>

<div class="there are spaces here"> this is there are spaces here </div>
<div class="there|are|special|characters|here"> this is there|are|special|characters|here </div>
<div class="You should use curly braces for inputs that aren't a single, continous word"> this is You should use curly braces for inputs that aren't a single, continous word  </div>
</body>
```

Potential improvements that could be made:
1. As of right now, the code lexes the file input into tokens, then parses the tokens twice. One to obtain all of the macros, and another time to obtain all of the macro calls(instances). This could potentially be streamlined into a single pass, however it would be at the cost of having to disambiguate the instance grammar, which would reduce the simplicity of the syntax.
2. There are several places where vectors are used that a map could potentially be more suitable, however some napkin math indicates that for smaller inputs, the cache locality of iterating over a vector heavily beats out the O(1) access times of an unordered map.
