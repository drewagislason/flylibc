# Markdown File With All FlyMarkdown Features

![flydoc Logo](flydoclogo.png "w3-circle")

# Heading 1

## Heading 2

#### Heading 3

#### Heading 4

##### Heading 5

###### Heading 6

Heading 1 Alternate
===================

Heading 2 Alternate
-------------------

A single line paragraph.

Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore
et dolore magna aliqua. Eu lobortis elementum nibh tellus molestie nunc non. Egestas erat imperdiet
sed euismod nisi porta. Et malesuada fames ac turpis egestas maecenas pharetra convallis posuere.
Neque aliquam vestibulum morbi blandit cursus. Nec nam aliquam sem et. Malesuada bibendum arcu
vitae elementum curabitur vitae nunc sed. Quam pellentesque nec nam aliquam. Laoreet id donec
ultrices tincidunt arcu non sodales. Elit eget gravida cum sociis natoque penatibus et magnis.
Iaculis at erat pellentesque adipiscing commodo elit at. Sollicitudin nibh sit amet commodo nulla
facilisi. Vulputate odio ut enim blandit. Elementum eu facilisis sed odio morbi quis commodo odio.
Lobortis mattis aliquam faucibus purus in massa tempor. Interdum velit euismod in pellentesque
massa placerat duis. In ornare quam viverra orci sagittis. Arcu non odio euismod lacinia at quis.
Quam id leo in vitae turpis massa sed elementum.

A paragraph with *italics*, *bold**, ***bold & italics***, ==highlight==, ~~strike through~~ and
`inline code`. Subscript and superscript look like this: H~2~O and x^2^.

Markdown supports emoji 😀 via UTF-8! The "hello 🌎" of Unicode code points.
See [Wikpedia Unicode Characters](https://en.wikipedia.org/wiki/List_of_Unicode_characters) and
[Wikipedia UTF-8](https://en.wikipedia.org/wiki/UTF-8).

A paragraph with [Reference to Chapter 1](#Chapter-1) and [section 1.2](#1.2-More-Ipsum-Lorem) and even
a [^footnote].

You can also quick link to a URL <https://search.brave.com>. This will leave the page.

1. A list
2. With just
3. Three lines

* [ ] A Task List
* [ ] with only one item
* [x] completed

1. An indented more complex list
  - Sublist 1
  + Sublist 2
    1. level 3a
    1. level 3b
  * Sublist 3
99. And bad numbering
3. Numbers are corrected

\100. Not a list  
\101. And neither is this

Block quote with 3 paragraphs.

> block quote
>
> with Three paragraphs
>
> One with *italics*, *bold**, ***bold & italics***, ==highlight==, ~~strike through~~ and
> `inline code`. Subscript and superscript look like this: H~2~O and x^2^.

Block quote, many lines, one paragraph:

> Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore
> et dolore magna aliqua. Eu lobortis elementum nibh tellus molestie nunc non. Egestas erat imperdiet
> sed euismod nisi porta. Et malesuada fames ac turpis egestas maecenas pharetra convallis posuere.
> Neque aliquam vestibulum morbi blandit cursus. Nec nam aliquam sem et. Malesuada bibendum arcu
> vitae elementum curabitur vitae nunc sed. Quam pellentesque nec nam aliquam. Laoreet id donec
> ultrices tincidunt arcu non sodales. Elit eget gravida cum sociis natoque penatibus et magnis.
> Iaculis at erat pellentesque adipiscing commodo elit at. Sollicitudin nibh sit amet commodo nulla
> facilisi. Vulputate odio ut enim blandit. Elementum eu facilisis sed odio morbi quis commodo odio.
> Lobortis mattis aliquam faucibus purus in massa tempor. Interdum velit euismod in pellentesque
> massa placerat duis. In ornare quam viverra orci sagittis. Arcu non odio euismod lacinia at quis.
> Quam id leo in vitae turpis massa sed elementum.

Multi-level block quote:

> level1, line 1
>> level2, line 1  
>> level2, line 2  
>>> level3, line 1  
>>> level3, line 2  
>>> level3, line 3  
> level1, line 2  
> level1, line 3

Some C code:

```c
void change_file_ext(char *path, const char *new_ext)
{
  char * old_ext = file_ext(path);
  if(old_ext)
    *old_ext = '\0';
  strcat(path, new_ext);
}
```

**Table Left/Right/Center**

left|center|right
:---|:----:|----:
a   |   b  |    c

**Table Grid 4**

Col 1 | Col 2 | Col 3 | Col 4
--- | --- | --- | ---
a|b|c|d
e|f|g|h

**Table Emoji & Emphasis**

emoji | code | superscript
-------- | --------- | -------------
emoji 🌎 |  `code` | ^superscript^

**Table Many Columns**

a|b|c|d|e|f|g|h|i|j|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z
---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---
1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16|17|18|19|20|21|22|23|24|25|26

## Chapter 1

Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore
et dolore magna aliqua. Eu lobortis elementum nibh tellus molestie nunc non. Egestas erat imperdiet
sed euismod nisi porta. Et malesuada fames ac turpis egestas maecenas pharetra convallis posuere.
Neque aliquam vestibulum morbi blandit cursus. Nec nam aliquam sem et. Malesuada bibendum arcu
vitae elementum curabitur vitae nunc sed. Quam pellentesque nec nam aliquam. Laoreet id donec
ultrices tincidunt arcu non sodales. Elit eget gravida cum sociis natoque penatibus et magnis.
Iaculis at erat pellentesque adipiscing commodo elit at. Sollicitudin nibh sit amet commodo nulla
facilisi. Vulputate odio ut enim blandit. Elementum eu facilisis sed odio morbi quis commodo odio.
Lobortis mattis aliquam faucibus purus in massa tempor. Interdum velit euismod in pellentesque
massa placerat duis. In ornare quam viverra orci sagittis. Arcu non odio euismod lacinia at quis.
Quam id leo in vitae turpis massa sed elementum.

## 1.2 More Ipsum Lorem

Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore
et dolore magna aliqua. Eu lobortis elementum nibh tellus molestie nunc non. Egestas erat imperdiet
sed euismod nisi porta. Et malesuada fames ac turpis egestas maecenas pharetra convallis posuere.
Neque aliquam vestibulum morbi blandit cursus. Nec nam aliquam sem et. Malesuada bibendum arcu
vitae elementum curabitur vitae nunc sed. Quam pellentesque nec nam aliquam. Laoreet id donec
ultrices tincidunt arcu non sodales. Elit eget gravida cum sociis natoque penatibus et magnis.
Iaculis at erat pellentesque adipiscing commodo elit at. Sollicitudin nibh sit amet commodo nulla
facilisi. Vulputate odio ut enim blandit. Elementum eu facilisis sed odio morbi quis commodo odio.
Lobortis mattis aliquam faucibus purus in massa tempor. Interdum velit euismod in pellentesque
massa placerat duis. In ornare quam viverra orci sagittis. Arcu non odio euismod lacinia at quis.
Quam id leo in vitae turpis massa sed elementum.

---

[^footnote]: Contains some definition of something

[^2]:        2nd footnote  
             Which has more lines in its  
             Paragraph.  

[^three]:    Footnote not referenced anywhere, but could be...
