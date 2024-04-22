add to rules.mk

```
REPEAT_KEY_ENABLE = yes
```

if you want to use get_hold_on_other_key_press
you need to this to conif.h

```
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
```

to set up qmk
https://docs.qmk.fm/#/newbs_getting_started

```
qmk setup
```

to build

```
make voyager:roei

- how to orgenized repo: https://filterpaper.github.io/qmk/userspace.html
```
