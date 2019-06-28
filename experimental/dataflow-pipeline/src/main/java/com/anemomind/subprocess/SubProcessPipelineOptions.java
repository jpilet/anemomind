
package com.anemomind.subprocess;

import com.anemomind.subprocess.configuration.SubProcessConfiguration;
import org.apache.beam.sdk.options.Default;
import org.apache.beam.sdk.options.DefaultValueFactory;
import org.apache.beam.sdk.options.Description;
import org.apache.beam.sdk.options.PipelineOptions;
import org.apache.beam.sdk.options.Validation.Required;

/** Options for running a sub process within a DoFn. */
public interface SubProcessPipelineOptions extends PipelineOptions {

  @Description("Source GCS directory where the C++ library is located e.g. gs://bucket/tests")
  @Required
  String getSourcePath();

  void setSourcePath(String sourcePath);

  @Description("Working directory for the process I/O")
  @Default.String("/tmp/grid_working_files")
  String getWorkerPath();

  void setWorkerPath(String workerPath);

  @Description("The maximum time to wait for the sub-process to complete")
  @Default.Integer(3600)
  Integer getWaitTime();

  void setWaitTime(Integer waitTime);

  @Description("As sub-processes can be heavy weight define the level of concurrency level")
  @Required
  Integer getConcurrency();

  void setConcurrency(Integer concurrency);

  @Description("Should log files only be uploaded if error.")
  @Default.Boolean(true)
  Boolean getOnlyUpLoadLogsOnError();

  void setOnlyUpLoadLogsOnError(Boolean onlyUpLoadLogsOnError);

  @Default.InstanceFactory(SubProcessConfigurationFactory.class)
  SubProcessConfiguration getSubProcessConfiguration();

  void setSubProcessConfiguration(SubProcessConfiguration configuration);

  @Description("mongodb uri for anemomind")
  @Default.String("mongodb://localhost:27017/anemomind-dev")
  String getMongoDbUri();

  void setMongoDbUri(String mongoDbUri);


  @Description("PubSub topic name for boat data")
  @Default.String("projects/anemomind/topics/anemomind_log_topic")
  String getTopic();

  void setTopic(String topic);

  @Description("Bucket name that contains all boat directories")
  @Default.String("boat_logs")
  String getBucketName();

  void setBucketName(String bucketName);

  @Description("Bucket name that contains directory for all SOs of c++ binaries ")
  @Default.String("boat_binaries")
  String getSoBucket();

  void setSoBucket(String soBucket);

  /** Confirm Configuration and return a configuration object used in pipeline. */
  class SubProcessConfigurationFactory implements DefaultValueFactory<SubProcessConfiguration> {
    @Override
    public SubProcessConfiguration create(PipelineOptions options) {

      SubProcessPipelineOptions subProcessPipelineOptions = (SubProcessPipelineOptions) options;

      SubProcessConfiguration configuration = new SubProcessConfiguration();

      if (subProcessPipelineOptions.getSourcePath() == null) {
        throw new IllegalStateException("Source path must be set");
      }
      if (subProcessPipelineOptions.getConcurrency() == null
              || subProcessPipelineOptions.getConcurrency() == 0) {
        throw new IllegalStateException("Concurrency must be set and be > 0");
      }
      configuration.setSourcePath(subProcessPipelineOptions.getSourcePath());
      configuration.setWorkerPath(subProcessPipelineOptions.getWorkerPath());
      configuration.setWaitTime(subProcessPipelineOptions.getWaitTime());
      configuration.setOnlyUpLoadLogsOnError(subProcessPipelineOptions.getOnlyUpLoadLogsOnError());
      configuration.concurrency = subProcessPipelineOptions.getConcurrency();
      configuration.setMongoDbUri(subProcessPipelineOptions.getMongoDbUri());
      configuration.setTopic(subProcessPipelineOptions.getTopic());
      configuration.setBucketName(subProcessPipelineOptions.getBucketName());
      configuration.setSoBucket(subProcessPipelineOptions.getSoBucket());


      return configuration;
    }
  }
}