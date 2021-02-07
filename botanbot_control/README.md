Refer to params/controller_server_config.yaml for configuring controller server. 

```yaml
botanbot_controller_server_rclcpp_node:
  ros__parameters:
    controller_plugin: "MPCControllerROS" # other options: 
    expected_controller_frequency: 5.0
    MPCControllerROS:
      plugin: "mpc_controller::MPCControllerROS"
      N: 10                                                       #timesteps in MPC Horizon
      DT: 0.1                                                     #discretization time between timesteps(s)    
      L_F: 0.66                                                   #distance from CoG to front axle(m)
      L_R: 0.66                                                   #distance from CoG to rear axle(m)
      V_MIN: -2.0                                                 #min / max velocity constraint(m / s)
      V_MAX:  2.0
      A_MIN:  -1.0                                                #min / max acceleration constraint(m / s ^ 2)
      A_MAX:   1.0
      DF_MIN:  -0.5                                               #min / max front steer angle constraint(rad)
      DF_MAX:   0.5
      A_DOT_MIN: -1.0                                             #min / max jerk constraint(m / s ^ 3)
      A_DOT_MAX:  1.0
      DF_DOT_MIN: -0.4                                            #min / max front steer angle rate constraint(rad / s)
      DF_DOT_MAX: 0.4
      Q: [10.0, 10.0, 10.0, 0.0]                                  #weights on x, y, psi, and v.
      R: [10.0, 100.0]                                            #weights on jerk and slew rate(steering angle derivative)
      debug_mode: True                                            #enable/disable debug messages
      params_configured: True                                     #set this true only if user figured the configration
   
```

Select a plugin out of 31 available plugins listed as ; MPCControllerROS. 

Launch the controller server with; 

```bash
ros2 launch botanbot_control controller_server.launch.py
```

Test the controller by sending an all zeros path with ;
 
```bash
ros2 action send_goal /follow_path botanbot_msgs/action/FollowPath "{}"
``` 