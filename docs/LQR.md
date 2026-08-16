
Consider the robot as a linear system modeled using a state space equation.

$$\dot{x} = Ax + Bu$$


This is a common method of mathematically modeling a system where...

$\dot{x}$ is the drivative of the state vector of the system. In this case $x = [\theta, \dot{\theta}, x, \dot{x}]$ being the angle, position, and their derivatives.

$A$ is the matrix defining how the system reacts based on the laws of physics. Basically how fast will it fall/roll away if we do nothing.

$B$ is the matrix representing how the system responds to control effort. What will happen to the system given motor response $u$.

We will define $A$ and $B$ using newtonian physics.

The goal of LQR is to find the optimal value for $u$ given any state $x$.

To define optimal we will need a cost function.

$$J = \int_{0}^{\infty} (x^T Q x + u^T R u) dt$$

$x^T Q x$ where Q is a symmetric positive semidefinite matrix works out to be... 
$$(Q_1x_1)^2+(Q_2x_2)^2+(Q_3x_3)^2+(Q_4x_4)^2$$

This is simply the normalized weighted sum of the deviation $x$.

Similarly,
$u^T T u$ where T is a symmetric positive definite matrix represents the the control effort (how much work the system does).

So this giant integral
$$J = \int_{0}^{\infty} (x^T Q x + u^T R u) dt$$
works out to how much work + how much error summed over time. Or in other words: If we let the system run forever, how optimal would it be. Smaller = better.

The nice part here is we get to define $Q$ and $R$ to mathmaticall define how much we care about fixing error and control effor respectivley.

So what we want is

$$min[J = \int_{0}^{\infty} (x^T Q x + u^T R u) dt]$$

Now if you did not already know.

You can find the minimum of a quadratic function $Ax^2 + Bx + C$ by finding the derivative and setting it to 0.

The same idea generalized to matricies produces the Algebraic Ricatti Equation:

$ATP+PA−PBR−1BTP+Q=0$

where the only unknown is $P$

This is basically *impossible* to derive and solve by hand but that's what computers are for.

After solving we get the optimal gain matric $K$ in the form.

$$K=R−1BTP$$

In the python script this will be

```python
scipy.linalg.solve_continuous_are(A, B, Q, R) 
K = inv(R) @ B.T @ P
```

Up until this point everything has been computed offline. Now all the MCU has to do live in the control loop is...
$$u = -Kx$$


I hope this made some sense because writing this out definetly helped me understand how im going to implement all this in the near future.

Feel free to ask if anything is unclear and I will try to answer you. Just keep in mind I am NOT a professor, just unemployed.

This was a very simple description of how LQR works. If this sparked your curiosity then check out this video from someone who really knows what they're talking about.

https://youtu.be/wEevt2a4SKI?si=k8IcvXmKzXowunIT