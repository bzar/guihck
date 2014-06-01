(define button
  (composite rectangle
    (prop 'width 64)
    (prop 'height 32)
    (prop 'color '(240 240 240))
    (alias 'on-clicked 'mouse-area 'on-mouse-down)
    (text
      (prop 'text "Button!")
      (align 'center)
      (prop 'color '(24 24 24)))
    (mouse-area
      (id 'mouse-area)
            (fill-parent)
      (method 'on-mouse-down (lambda (b x y)
        (set-prop! (parent) 'color '(255 255 255))))
      (method 'on-mouse-up (lambda (b x y)
        (set-prop! (parent) 'color '(240 240 240)))))))

(create-elements!
  (row
    (align 'top-left 100)
    (prop 'spacing 16)
    (column
      (prop 'spacing 8)
      (button)
      (button
        (prop 'width 128)))
    (column
      (prop 'spacing 8)
      (button)
      (button
        (prop 'height 64))
      (button
        (method 'on-clicked (lambda (b x y)
          (set-prop! 'color '(255 0 0))))))))