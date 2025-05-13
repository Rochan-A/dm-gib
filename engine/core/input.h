namespace core {

// Controls behavior when Esc key is pressed.
enum class EscBehavior {
  NONE,
  TOGGLE_MOUSE_CAPTURE,
  CLOSE,
  // Uncaptures the mouse if it is captured, or else closes the app.
  UNCAPTURE_MOUSE_OR_CLOSE,
};

// Controls behavior when the LMB is pressed.
enum class MouseButtonBehavior {
  NONE,
  CAPTURE_MOUSE,
};

class InputManager {
public:
  InputManager() = default;

  ~InputManager() = default;

  InputManager(InputManager &&other) noexcept;
  InputManager &operator=(InputManager &&other) noexcept;

  InputManager(const InputManager &) = delete;
  const InputManager &operator=(const InputManager &) = delete;

private:
};

} // namespace core