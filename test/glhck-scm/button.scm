(define button
  (composite rectangle '(width 64 height 32 color (240 240 240))
    (text '(text "Button!" x 8 y 8 color (24 24 24)))
    (mouse-area '(width 64 height 32
                  onMouseDown ((with-parent) (lambda () 
                    (set-element-property! 'color '(255 255 255))))
                  onMouseUp ((with-parent) (lambda () 
                    (set-element-property! 'color '(240 240 240))))))))
                    
(create-elements
  (row '(x 100 y 100 spacing 16)
    (column '(spacing 8)
      (button '())
      (button '()))
    (column '(spacing 8)
      (button '())
      (button '())
      (button '()))))