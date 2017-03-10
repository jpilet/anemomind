# Problematic data

  * boat55a774ac16361494ab094dc7 (SYZ)
    A few outlier positions here and there...

    - on 2015-09-05
    - on 2015-08-29

  * boat58b5fde26b146bd2da067681 (Courdileone)

    - on 2017-03-03: A few outliers here and there
    - on 2017-03-02T13:03:40 to 2017-03-02T16:18:40

  * Irene 

    - around Flensburg I think: The GPS trajectory was offset by a constant for some time.

# Related pull requests and issues

  * https://github.com/jpilet/anemomind/pull/1031, that removes some bad data, using two approaches: (i) By chopping up the position data into segments when there are large gaps, and then removing short segments and related motion data, and (ii) by identifying individual outliers that are surrounded by two inliers.



