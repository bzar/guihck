(define button
  (composite rectangle '(width 64
                         height 32
                         color (240 240 240)
                         on-clicked (alias (find-element 'mouse-area) on-mouse-down))
    (text '(text "Button!" 
            x (bind (/ (- (get-prop (parent) 'width) (get-prop 'width)) 2))
            y (bind (/ (- (get-prop (parent) 'height) (get-prop 'height)) 2)) 
            color (24 24 24)))
    (mouse-area '(id mouse-area
                  width (bind (get-prop (parent) 'width))
                  height (bind (get-prop (parent) 'height))
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