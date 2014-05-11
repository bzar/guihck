(define button
  (composite rectangle '(width 64 height 32 color (240 240 240))
    (text '(text "Button!" x 8 y 8 color (24 24 24)))
    (mouse-area '(width 64 height 32
                  on-mouse-down (set-prop! (parent!) 'color '(255 255 255))
                  on-mouse-up (set-prop! (parent!) 'color '(240 240 240))))))

(create-elements
  (row '(x 100 y 100 spacing 16)
    (column '(spacing 8)
      (button '())
      (button '()))
    (column '(spacing 8)
      (button '())
      (button '())
      (button '()))))
 