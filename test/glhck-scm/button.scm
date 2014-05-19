(define button
  (composite rectangle '(width 64
                         height 32
                         color (240 240 240)
                         on-clicked (alias (find-element 'mouse-area) 'on-mouse-down))
    (text '(text "Button!" 
            x (bind (observe 'this 'width 'parent 'width) (lambda (w pw) (/ (- pw w) 2)))
            y (bind (observe 'this 'height 'parent 'height) (lambda (h ph) (/ (- ph h) 2)))
            color (24 24 24)))
    (mouse-area '(id mouse-area
                  width (bind (observe 'parent 'width) identity)
                  height (bind (observe 'parent 'height) identity)
                  on-mouse-down (set-prop! (parent) 'color '(255 255 255))
                  on-mouse-up (set-prop! (parent) 'color '(240 240 240))))))

(create-elements!
  (row '(x 100 y 100 spacing 16)
    (column '(spacing 8)
      (button '())
      (button '(width 128)))
    (column '(spacing 8)
      (button '())
      (button '(height 64))
      (button '(on-clicked (set-prop! (parent) 'color '(255 0 0)))))))