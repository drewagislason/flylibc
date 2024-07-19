# Test for Md2Html Module

This contains an example of all features of markdown, along with corner cases.

## Headings
Text immediately after heading OK

Head To <a href="duckduckgo.com">DuckDuckGo</a>
===

Heading 2
----

### #include \<stdio.h>

### heading 3

#### heading 4

##### heading 5

###### heading 6

text of a normal paragraph

## Paragraphs

basic paragraph
with two markdown lines, but one HTML pararaph.

This paragraph has `code`, an image ![alt](md2html_image.png) a [reference](md2html.html) a break  
and some < tranformed & characters.

Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Turpis tincidunt id aliquet risus feugiat in ante metus. Eu ultrices vitae auctor eu augue ut. Fusce id velit ut tortor pretium viverra. Quam lacus suspendisse faucibus interdum posuere. Massa sapien faucibus et molestie ac feugiat sed lectus vestibulum. Quis vel eros donec ac odio tempor orci. Mattis enim ut tellus elementum sagittis vitae. Vehicula ipsum a arcu cursus vitae congue. Velit sed ullamcorper morbi tincidunt ornare massa eget egestas. Nibh mauris cursus mattis molestie a iaculis at. Risus commodo viverra maecenas accumsan lacus. Sit amet consectetur adipiscing elit ut aliquam purus sit amet. Eget aliquet nibh praesent tristique magna sit amet. Aliquam etiam erat velit scelerisque in dictum non consectetur. Tortor vitae purus faucibus ornare suspendisse sed. Amet dictum sit amet justo donec enim diam. Diam in arcu cursus euismod quis.

test of \ single escape char or \. escape other char or \\ or even < or > or &

<p>my own paragraph <b>bolded</b></p>

Does it look OK, or too many spaces?


## Lists

Note: some of these lists do not appear correctly in MacDown, which doesn't exactly follow markdown spec.

Note also, Ms2Html Library doesn't support paragraphs in lists. Instead

* simple list
+ with
- three lines

1. Nested List
    1. inside 1
        - wow 1
        - wow 2
    9. inside 2
    123. inside 3
2. Nested 2

List ended with this paragraph

1. List with paragraph
  
	Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut  
	labore et dolore magna aliqua. Adipiscing vitae proin sagittis nisl rhoncus mattis rhoncus  
	urna. Sed cras ornare arcu dui vivamus arcu. Id leo in vitae turpis. Sit amet porttitor eget  
	dolor morbi non arcu risus quis. Euismod elementum nisi quis eleifend quam adipiscing. Neque  
	volutpat ac tincidunt vitae semper quis. Suspendisse potenti nullam ac tortor vitae purus  
	faucibus. Sed id semper risus in hendrerit. Magnis dis parturient montes nascetur ridiculus  
	mus mauris. Id faucibus nisl tincidunt eget nullam. Nec ullamcorper sit amet risus nullam  
	eget. Sed risus ultricies tristique nulla aliquet enim tortor. Vitae semper quis lectus  
	nulla at volutpat diam.  

9. Item two

## Tables

1|
---
a|

1|2
---|---

a|b
---|---
c|d

a|b|
---|---
12\|3|456

Left           | Centered            | Right
:------------- | :-----------------: | --:
Joe JoJo       |                     | [42](https://lifetell.com/42-the-meaning-of-life/)
Jane N. Tarzan | Swings & yodels     | 19
Bob `code`     | **Floats in water** | 65

a|b|c
---|---|---
1|2|Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Tortor at risus viverra adipiscing at in tellus integer feugiat. Nullam vehicula ipsum a arcu cursus. Vestibulum lectus mauris ultrices eros. Neque viverra justo nec ultrices dui sapien. Condimentum vitae sapien pellentesque habitant. Faucibus turpis in eu mi bibendum neque egestas congue quisque. Volutpat sed cras ornare arcu dui vivamus arcu. Eleifend quam adipiscing vitae proin sagittis. Adipiscing enim eu turpis egestas pretium aenean pharetra. Eget est lorem ipsum dolor. Et malesuada fames ac turpis egestas maecenas pharetra convallis posuere. Lorem ipsum dolor sit amet. Nisl nisi scelerisque eu ultrices vitae. Aliquet lectus proin nibh nisl. Accumsan in nisl nisi scelerisque eu ultrices vitae auctor eu. Cursus euismod quis viverra nibh cras pulvinar mattis. Nibh nisl condimentum id venenatis a. Massa sed elementum tempus egestas.
5|6

a|b|c|d
---|---|---|---
one cell|||
cell2a|cell2b
cell3a|cell3b|cell3c|cell3d
not cell line

###### Smallest Header

####### Not a header

## HTML

It's OK to have HTML in your file

<link rel="stylesheet" href="https://www.w3schools.com/w3css/4/w3.css">
<img src="flydoclogo.png" class="w3-circle">
