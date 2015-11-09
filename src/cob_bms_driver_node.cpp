#include "cob_bms_driver/cob_bms_driver_node.h"

void CobBmsDriverNode::loadParameters()
{	 
	//declarations
    XmlRpc::XmlRpcValue diagnostics1, diagnostics2;
    std::vector <std::string> topics;
    
    if (!nh_.getParam("topics", topics)) 
    {
		ROS_INFO_STREAM("Did not find \"topics\" on parameter server");		
	}    
    loadTopics(topics);
    
    if (!nh_.getParam("diagnostics1", diagnostics1)) 
    {
		ROS_INFO_STREAM("Did not find \"diagnostics1\" on parameter server");		
	}
	//true for diagnostics1, false for diagnostics2
	loadConfigMap(diagnostics1, true);	
	
	 if (!nh_.getParam("diagnostics2", diagnostics2)) 
	 {
		ROS_INFO_STREAM("Did not find \"diagnostics2\" on parameter server");		
	}
	loadConfigMap(diagnostics2, false);
}

void CobBmsDriverNode::loadTopics(std::vector<std::string> topics) 
{	
	topics_ = topics;
	for (int i=0 ; i<topics.size(); ++i) 
	{
		ROS_INFO_STREAM("topics: " << topics.at(i));
	}
}

void CobBmsDriverNode::loadConfigMap(XmlRpc::XmlRpcValue config_l0_array, bool is_diagnostic1) 
{
	XmlRpc::XmlRpcValue config_l1_struct, config_l2, config_l3_struct, xdiagnostics, xdiagnostic_elements, xfields, xpair,temp;
    BmsParameter bms_parameter;
    char id;		
    
    ROS_ASSERT(config_l0_array.getType() == XmlRpc::XmlRpcValue::TypeArray);  
	for (int32_t i = 0; i < config_l0_array.size(); ++i) 
	{	
		ROS_ASSERT(config_l0_array[i].getType() == XmlRpc::XmlRpcValue::TypeStruct);
		config_l1_struct = config_l0_array[i];		
		for (XmlRpc::XmlRpcValue::iterator it1=config_l1_struct.begin(); it1!=config_l1_struct.end(); ++it1) 
		{
			config_l2 = it1->second;
			if (config_l2.getType()==XmlRpc::XmlRpcValue::TypeInt) 
			{				
				id = static_cast<char>(static_cast<int>(config_l2));
			}
			else if (config_l2.getType()==XmlRpc::XmlRpcValue::TypeArray) 
			{			
				for(int32_t j=0; j<config_l2.size(); ++j) 
				{
					ROS_ASSERT(config_l2[j].getType()==XmlRpc::XmlRpcValue::TypeStruct);
					config_l3_struct = config_l2[j];
					for (XmlRpc::XmlRpcValue::iterator it3=config_l3_struct.begin(); it3!=config_l3_struct.end(); ++it3) 
					{
						if (it3->first == "name") 
						{
							ROS_ASSERT(it3->second.getType()==XmlRpc::XmlRpcValue::TypeString);
							bms_parameter.name = static_cast<std::string>(it3->second);
						}
						else if (it3->first == "offset") 
						{
							ROS_ASSERT(it3->second.getType()==XmlRpc::XmlRpcValue::TypeInt);
							bms_parameter.offset = static_cast<int>(it3->second);
						}
						else if (it3->first == "len") 
						{
							ROS_ASSERT(it3->second.getType()==XmlRpc::XmlRpcValue::TypeInt);
							bms_parameter.length = static_cast<int>(it3->second);
						}
						else if (it3->first == "is_signed") 
						{
							ROS_ASSERT(it3->second.getType()==XmlRpc::XmlRpcValue::TypeBoolean);
							bms_parameter.is_signed = static_cast<bool>(it3->second);
						}
						else if (it3->first == "factor") 
						{
							ROS_ASSERT(it3->second.getType()==XmlRpc::XmlRpcValue::TypeDouble);
							bms_parameter.factor = static_cast<double>(it3->second);
						}
						else if (it3->first == "unit") 
						{
							ROS_ASSERT(it3->second.getType()==XmlRpc::XmlRpcValue::TypeString);
							bms_parameter.unit = static_cast<std::string>(it3->second);
						} 
						else ROS_ERROR_STREAM("Unexpected Key: " << it3->first);
					}
				}
			}
			else ROS_ERROR_STREAM("Config: Expected either XmlRpc Type: " << XmlRpc::XmlRpcValue::TypeArray << " or " << XmlRpc::XmlRpcValue::TypeInt << ". But found: " << config_l2.getType());
		}
		//save bms_parameter in config map for interpreting the parameters and diagnostics
		config_map_[id] = bms_parameter;
		//also save the id in parameter list for polling
		is_diagnostic1 ? param_list1_.push_back(id) : param_list2_.push_back(id);
	}
}

void CobBmsDriverNode::pollNextInParamLists()
{
	//return to start if reached the end of parameter lists
	if (param_list1_it_ == param_list1_.end()) param_list1_it_ = param_list1_.begin();
	if (param_list2_it_ == param_list2_.end()) param_list2_it_ = param_list2_.begin();
	
	ROS_INFO_STREAM("polling paramaters at ids: " <<*param_list1_it_ << " and " << *param_list2_it_);
	
	//poll
	bms_driver_.pollBmsforParameters(*param_list1_it_,*param_list1_it_); //
	
	//increment iterators for next poll 
	++param_list1_it_;
	++param_list2_it_;
}

bool CobBmsDriverNode::prepare() {
	
	//TODO: add check for initialization of driver before starting polls
	if (bms_driver_.initializeDriver() == false) {
		ROS_ERROR_STREAM("bms_driver initialization failed");
		return false;
	}
	
	loadParameters();
	//bms_driver_.setConfigMap(&config_map_);	
	
	//initalize parameter list iterators
	param_list1_it_ = param_list1_.begin();
	param_list2_it_ = param_list2_.begin();
	
	return true;
	
}

CobBmsDriverNode::CobBmsDriverNode() 
{
	//hardcoded parameter lists 
	//param_list1_.push_back(0x02); param_list1_.push_back(0x03); param_list1_.push_back(0x06);
	
	/*param_list2_.push_back(0x15); /*paramater_ids_list2_.push_back("0116"); paramater_ids_list2_.push_back("0117"); 
	paramater_ids_list2_.push_back("0118"); paramater_ids_list2_.push_back("0119"); paramater_ids_list2_.push_back("011A");
	paramater_ids_list2_.push_back("011B");*/
}

CobBmsDriverNode::~CobBmsDriverNode() {}

int main(int argc, char **argv) 
{		
	ros::init(argc, argv, "bms_driver_node");
		
	CobBmsDriverNode cob_bms_driver_node;
	if (!cob_bms_driver_node.prepare()) return 1;	

	while (cob_bms_driver_node.nh_.ok())
    {	
		cob_bms_driver_node.pollNextInParamLists();		 
		ros::spinOnce();
    }
    
    return 0;	
}
