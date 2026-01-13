# Fast Aerial Trainer / Jump Timing Trainer

This plugin shows some useful stats about your jump inputs.

You can also use it to practice different jump-related mechanics like **fast aerials**, **speed-flips** or **wall-dashes**.

Works in **freeplay**, **custom training** and with different **game-speeds**.

![fast-aerial-trainer-settings](https://github.com/user-attachments/assets/845e34a1-372f-4118-8d46-f0de880921c1)

It is based on the plugin [Fast Aerial Trainer](https://bakkesplugins.com/plugins/view/406) made by Vync.  
The original plugin was featured in the video [Wall Dash, Zap Dash, Fast Aerial, & More | How to Play Rocket League](https://www.youtube.com/watch?v=zbW7jIav2e8&t=728s) by [Grifflicious](https://www.youtube.com/@Grifflicious).

## How to Train Fast Aerials, Speed-Flips or Wall-Dashes

You can easily configure this plugin to aim for different success metrics in the plugin settings UI.

If you want to frequently change between different configurations, I recommend you use bindings or console aliases. Here's how:

All settings can also be configured in the console (opened with `F6`).

You can either paste the example commands below into the console, use them in the bindings tab or save them as an alias with `alias <your_alias> "<command>"` (so you can run the command `<your_alias>` in the console next time).  
Feel free to adjust them however it's useful for you, for example by changing the success timings.

**Example configuration for fast aerials**

```
fast_aerial_trainer_record_after_double_jump 0.5; fast_aerial_trainer_gui_show_first_jump 1; fast_aerial_trainer_gui_show_double_jump 1; fast_aerial_trainer_gui_show_total_jump 0; fast_aerial_trainer_gui_draw_pitch_history 1; fast_aerial_trainer_gui_draw_boost_history 1; fast_aerial_trainer_gui_jump_ranges '0,180,195,225,260,300'; fast_aerial_trainer_gui_double_jump_ranges '0,75,110,130'; 
```

**Example configuration for speed-flips**

```
fast_aerial_trainer_record_after_double_jump 0.2; fast_aerial_trainer_gui_show_first_jump 0; fast_aerial_trainer_gui_show_double_jump 0; fast_aerial_trainer_gui_show_total_jump 1; fast_aerial_trainer_gui_draw_pitch_history 1; fast_aerial_trainer_gui_draw_boost_history 1; fast_aerial_trainer_gui_total_jump_ranges '0,100,150,200'; 
```

**Example configuration for wall-dashes**

```
fast_aerial_trainer_gui_show_first_jump 0; fast_aerial_trainer_gui_show_double_jump 0; fast_aerial_trainer_gui_show_total_jump 1; fast_aerial_trainer_gui_draw_pitch_history 0; fast_aerial_trainer_gui_draw_boost_history 0; fast_aerial_trainer_gui_total_jump_ranges '0,75,90,120'; 
```


## Explanation of Values

### Initial Jump Duration

How long you held the jump bottom on your initial jump.  
The optimal value for fast aerials is 200ms. See [RLBot - Jumping Physics](https://github.com/RLBot/RLBot/wiki/Jumping-Physics).

### Time to Double Jump

How long it took you to press the jump button a second time after letting go of it.  
Faster is generally better.

### Total Jump Duration

How long it took you to press the jump button a second time after pressing it initially.  
`Total Jump Duration = Initial Jump Duration + Time to Double Jump`  
This metric is useful when practicing faster mechanics like speed-flips or wall-dashes.

### Pitch Up Between Jumps

Measures your pitch back input between leaving the ground and double-jumping. It takes into account aerial sensitivity.  
Higher is better: 100% means you pitched upwards fully. 50% could mean you pitched back half for the whole duration or you pitched back fully for half the time.

### Pitch History

This graph shows your pitch input.  
The horizontal axis represents time starting from the initial jump.  
The vertical axis shows your pitch up input between 0 and 1 (scaled with input sensitivity).  
The vertical line shows when you double jumped.  
This graph helps in improving your takeoff, because you can see where you missed crucial inputs.

### Boost History

This graph shows your boost input.  
The horizontal axis represents time starting from the initial jump.

### First Input Warning in Custom Training

Shows a warning when jump wasn't the first input in custom training.  
If you want to avoid steer or pitch inputs from starting the countdown, I recommend installing the plugin [Fix Custom Training Start](https://bakkesplugins.com/plugins/view/311).
A nice training pack to check your progress on fast aerials is ["Fast Aerial Test - Touch Ball" from Lazord](https://prejump.com/training-packs/B66E-6D43-7E93-F055): B66E-6D43-7E93-F055

## Improvements over the Original Plugin

- More settings. Especially color settings (because I'm color-blind).
- Settings are saved between sessions.
- Pitch history: A graph showing your pitch inputs.
- Better total tilt calculation: Considers aerial sensitivity and input amount now.
- Now works in slow-motion, using in-game speed instead of real-time.
- Bars fill up until input.

## Troubleshooting

Having an issue with the plugin? Found a bug? Missing a feature?  
Open a new issue on GitHub and I'll look into it.

## Changelog

See [CHANGELOG.md](./CHANGELOG.md).
